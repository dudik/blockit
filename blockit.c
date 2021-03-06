/** @file
 * Functions that mention that they are part of the WebKitGTK API are connected to signals which are described in more detail either here:  
 * https://webkitgtk.org/reference/webkit2gtk/stable/WebKitWebExtension.html  
 * or here:  
 * https://webkitgtk.org/reference/webkit2gtk/stable/WebKitWebPage.html
 *
 * Description includes description of function arguments.
 */
#include <webkit2/webkit-web-extension.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "resources/gschema.h"

int sock;
GtkWidget *settings;
GtkWidget *enabled;
JSCContext *js_context;
const gchar *zap_js;
const gchar *block_js;
const gchar *config_dir;

/**
 * WebKitGTK API callback function that gets called when a new request is about to be sent to the server ('send-request' signal). It's used to catch potentionally unwanted requests and blocking them if the filtering server deems them unwanted.
 */
static gboolean web_page_send_request(WebKitWebPage *web_page, WebKitURIRequest *request, WebKitURIResponse *redirected_response, gpointer user_data)
{
	if (!gtk_switch_get_state(GTK_SWITCH(enabled)))
		return FALSE;

	const char *request_uri = webkit_uri_request_get_uri(request);
	const char *page_uri = webkit_web_page_get_uri(web_page);

	gchar *path;
	g_uri_split(request_uri, G_URI_FLAGS_NONE, NULL, NULL, NULL, NULL, &path, NULL, NULL, NULL);

	// Simple algorithm that derives the type from a request
	const gchar *res_type;
	if (g_str_has_suffix(path, ".js"))
		res_type = "script";
	else if (g_str_has_suffix(path, ".css"))
		res_type = "stylesheet";
	else if (g_str_has_suffix(path, ".jpg") || g_str_has_suffix(path, ".png") || g_str_has_suffix(path, ".svg") || g_str_has_suffix(path, ".gif"))
		res_type = "image";
	else if (g_str_has_suffix(path, ".ttf") || g_str_has_suffix(path, ".otf") || g_str_has_suffix(path, ".woff") || g_str_has_suffix(path, ".woff2"))
		res_type = "font";
	else
		res_type = "other";

	g_free(path);

	gchar *req = g_strdup_printf("n %s %s %s\n", request_uri, page_uri, res_type);
	write(sock, req, strlen(req));
	g_free(req);
	gchar buffer[1] = {0};
	read(sock, buffer, 1);
	// Server returns '1' if the resource is an ad.
	return buffer[0] == '1';
}

/**
 * WebKitGTK API callback function that gets called when the DOM document of a webpage has been loaded (signal 'document-loaded'). Used for opening the settings window, but also for getting lists of ids and classes that are present on the given webpage which are sent to the server for filtering. The resulting CSS rule is also injected into the webpage through this function.
 */
static void document_loaded_callback(WebKitWebPage *web_page, gpointer user_data)
{
	const gchar *uri = webkit_web_page_get_uri(web_page);

	if (g_str_has_suffix(uri, "blockit://settings") || g_str_has_suffix(uri, "blockit//settings"))
		gtk_widget_show_all(settings);

	if (!gtk_switch_get_state(GTK_SWITCH(enabled)))
		return;

	// Get classes and ids present on the webpage and send them to the server for filtering.
	JSCValue *classes = jsc_context_evaluate(js_context, "Array.from(new Set([].concat.apply([], Array.from(document.getElementsByTagName('*')).map(elem => Array.from(elem.classList))))).join('\t')", -1);
	JSCValue *ids = jsc_context_evaluate(js_context, "Array.from(document.getElementsByTagName('*')).map(elem => elem.id).filter(elem => elem).join('\t')", -1);
	gchar *req = g_strdup_printf("c %s %s %s\n", uri, jsc_value_to_string(ids), jsc_value_to_string(classes));
	write(sock, req, strlen(req));
	g_free(req);

	GString *res = g_string_new("");
	char *end = NULL;

	do
	{
		gchar buffer[1024] = {0};
		read(sock, buffer, 1023);
		end = strchr(buffer, '\n');
		if (end)
			*end = 0;
		g_string_append(res, buffer);
	} while (!end);

	if (res->len > 1)
	{
		// If the server response contains a CSS rule, inject it into the webpage.
		g_string_prepend(res, "var link = document.createElement('link');"
				"link.rel = 'stylesheet';"
				"link.href = 'a';"
				"document.head.appendChild(link);"
				"window.onload = function () { link.sheet.insertRule(`");
		g_string_append(res, "`); }");
		jsc_context_evaluate(js_context, res->str, -1);
		g_string_free(res, TRUE);
	}
}

/**
 * WebKitGTK API callback function that gets called when a new message is sent to the Javascript console ('console-message-sent' signal). Used for catching a specific message from the 'block element' interface that specifies the filter rule that should be added to the custom filter lists file for permanent blocking.
 */
static void console_message_sent_callback(WebKitWebPage *web_page, WebKitConsoleMessage *console_message, gpointer user_data) {
	const gchar *msg = webkit_console_message_get_text(console_message);

	if (g_str_has_prefix(msg, "blockit ")) {
		// Append cosmetic rule to custom filters.
		const char *page_uri = webkit_web_page_get_uri(web_page);

		gchar *path;
		g_uri_split(page_uri, G_URI_FLAGS_NONE, NULL, NULL, &path, NULL, NULL, NULL, NULL, NULL);

		gchar *uri = g_strconcat("file:///", g_get_user_config_dir(), "/ars/lists/custom", NULL);
		GFile *file = g_file_new_for_uri(uri);
		g_free(uri);
		GFileOutputStream *stream = g_file_append_to(file, G_FILE_CREATE_NONE, NULL, NULL);
		gchar *buf = g_strconcat("\n", path, msg + 8, "\n", NULL);
		g_output_stream_write(G_OUTPUT_STREAM(stream), buf, strlen(buf), NULL, NULL);
		g_free(buf);
	}
}

/**
 * WebKitGTK API callback function that gets called everytime a WebKitWebPage is created ('page-created' signal). Used for getting the main frame and Javascript context of the webpage. Also for connecting to other signals - 'send-request', 'document-loaded' and 'console-message-sent'.
 */
static void web_page_created_callback(WebKitWebExtension *extension, WebKitWebPage *web_page, gpointer user_data)
{
	WebKitFrame *frame = webkit_web_page_get_main_frame(web_page);
	js_context = webkit_frame_get_js_context(frame);

	g_signal_connect(web_page, "send-request", G_CALLBACK(web_page_send_request), NULL);
	g_signal_connect(web_page, "document-loaded", G_CALLBACK(document_loaded_callback), NULL);
	g_signal_connect(web_page, "console-message-sent", G_CALLBACK(console_message_sent_callback), NULL);
}

/**
 * Hides the settings window.
 * @param widget Widget that triggered this function, in this use case it's always empty.
 * @param data Optional user data.
 */
void hide_settings(GtkWidget *widget, gpointer *data)
{
	gtk_widget_hide(widget);
}

/**
 * Injects Javascript code to the currently displayed website that will start the 'zap element' interface.
 * @param button The button widget that triggered this function.
 * @param data Optional user data.
 */
void pick_elem(GtkWidget *button, gpointer *data)
{
	jsc_context_evaluate(js_context, zap_js, -1);
}

/**
 * Injects Javascript code to the currently displayed website that will start the 'block element' interface.
 * @param button The button widget that triggered this function.
 * @param data Optional user data.
 */
void block_elem(GtkWidget *button, gpointer *data)
{
	jsc_context_evaluate(js_context, block_js, -1);
}

/**
 * Opens the filter lists configuration file in the default text editor.
 * @param button The button widget that triggered this function.
 * @param data Optional user data.
 */
void edit_lists(GtkWidget *button, gpointer *data)
{
	GError *error = NULL;
	gchar *uri = g_strconcat("file:///", g_get_user_config_dir(), "/ars/urls", NULL);

	if (!g_app_info_launch_default_for_uri(uri, NULL, &error))
		g_warning("Failed to open filter lists file: %s", error->message);

	g_free(uri);
}

/**
 * Opens the custom filters configuration file in the default text editor.
 * @param button The button widget that triggered this function.
 * @param data Optional user data.
 */
void edit_filters(GtkWidget *button, gpointer *data)
{
	GError *error = NULL;
	gchar *uri = g_strconcat("file:///", g_get_user_config_dir(), "/ars/lists/custom", NULL);

	if (!g_app_info_launch_default_for_uri(uri, NULL, &error))
		g_warning("Failed to open custom filters file: %s", error->message);

	g_free(uri);
}

/**
 * Sends a request to the server requiring to reload the server.
 * @param button The button widget that triggered this function. Used to 'disable' the button while waiting for a response from the server.
 * @param data Optional user data.
 */
void reload_server(GtkWidget *button, gpointer *data)
{
	gtk_widget_set_sensitive(button, FALSE);
	write(sock, "r\n", 2);
	gchar buffer[1] = {0};
	read(sock, buffer, 1);
	gtk_widget_set_sensitive(button, TRUE);
}

/**
 * Sends a request to the server requiring to forcefully update every active filter.
 * @param button The button widget that triggered this function. Used to 'disable' the button while waiting for a response from the server.
 * @param data Optional user data.
 */
void force_update(GtkWidget *button, gpointer *data)
{
	gtk_widget_set_sensitive(button, FALSE);
	write(sock, "u\n", 2);
	gchar buffer[1] = {0};
	read(sock, buffer, 1);
	gtk_widget_set_sensitive(button, TRUE);
}

/**
 * WebKitGTK API function that gets called when the extension is initialized. It's the entry point of the extension. It's used for connecting to the server socket. If it's not running, it will try to automatically start it if possible.
 */
G_MODULE_EXPORT void webkit_web_extension_initialize(WebKitWebExtension *extension)
{
	sock = socket(AF_UNIX, SOCK_STREAM, 0);

	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	memset(addr.sun_path, 0, sizeof(addr.sun_path));
	strcpy(addr.sun_path, "/tmp/ars");

	// Spawn server if it's not running.
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		gchar *argv[] = {"adblock-rust-server", NULL};
		gint out;

		// "disable" extension if server can't be started.
		if (!g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, &out, NULL, NULL)) {
			GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE, "adblock-rust-server could not be started");
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "adblock-rust-server isn't running and couldn't be started. Please check that it is installed and accessible from $PATH. You can install it with cargo: 'cargo install adblock-rust-server'.");
			gtk_widget_show(dialog);
			return;
		}

		// Wait until server prints out the 'init-done' line and try connecting to the server again.
		GIOChannel *out_chan = g_io_channel_unix_new(out);
		gchar *line;
		g_io_channel_read_line(out_chan, &line, NULL, NULL, NULL);
		g_free(line);
		g_io_channel_unref(out_chan);
		connect(sock, (struct sockaddr *)&addr, sizeof(addr));
	}

	config_dir = g_get_user_config_dir();

	GBytes *bytes = g_resources_lookup_data("/resources/scripts/zap.js", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
	zap_js = g_bytes_get_data(bytes, NULL);

	bytes = g_resources_lookup_data("/resources/scripts/block.js", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
	block_js = g_bytes_get_data(bytes, NULL);

	GtkBuilder *builder = gtk_builder_new_from_resource("/resources/gui.glade");
	gtk_builder_connect_signals(builder, NULL);
	settings = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	enabled = GTK_WIDGET(gtk_builder_get_object(builder, "enabled"));

	g_signal_connect(extension, "page-created", G_CALLBACK(web_page_created_callback), NULL);
}
