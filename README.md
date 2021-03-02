# blocKit
## Installation
1. Install [adblock-rust-server](https://crates.io/crates/adblock-rust-server):
```shell
cargo install adblock-rust-server
```
2. Clone this repository and install:
```
git clone https://github.com/dudik/blockit
cd blockit
sudo make install
```
3. Link the library located at `/usr/lib/blockit/blockit.so` with the WebKitGTK based browser you want to use:
```
sudo ln -s /usr/lib/blockit/blockit.so /usr/lib/example-browser
```
The target folder is different for each browser. The locations can be found at [wyebadblock's Github page](https://github.com/jun7/wyebadblock#addition-for-other-webkit2gtk-browsers).

Before starting your browser you have to manually start the adblock server and keep it running while using the browser. If you don't then the extension will get stuck in a loop and your browser won't load any pages. Just close your browser and start the server:
```
adblock-rust-server
```
This is temporary and will get fixed soon.

Filter lists are located in `~/.config/ars/lists`. By default the folder is empty. You can use lists like [easylist](https://easylist.to/easylist/easylist.txt) or [easyprivacy](https://easylist.to/easylist/easyprivacy.txt). Just put them in the directory and restart the server.
