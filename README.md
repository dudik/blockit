# blocKit
> WebKitGTK adblock extension with Brave's Rust-based adblock engine for backend.

## Features
* Network and cosmetic filtering
* Supports Adblock Plus filter syntax
* Automatic filter lists updating
* Element picker/zapper
* Simple GUI

## Installation
1. Install [adblock-rust-server](https://crates.io/crates/adblock-rust-server):
```shell
cargo install adblock-rust-server
```
2. Clone this repository and install:
```shell
git clone https://github.com/dudik/blockit
cd blockit
sudo make install
```
3. Link the library located at `/usr/local/lib/blockit.so` with the WebKitGTK based browser you want to use:
```shell
sudo ln -s /usr/local/lib/blockit.so /usr/lib/example-browser
```
The target folder is different for each browser. The locations can be found at [wyebadblock's Github page](https://github.com/jun7/wyebadblock#addition-for-other-webkit2gtk-browsers).

The server should start automatically when you open your browser. If not, try starting it manually:
```shell
adblock-rust-server
```

## Usage
<img align="right" width="161.6" height="276" src="https://user-images.githubusercontent.com/24730635/113864606-6420b900-97ab-11eb-9735-70adfed4d4dd.png">The GUI can be opened by navigating your browser to `blockit://settings`.

The **Zap element** button allows the user to remove any element on the webpage with a left mouse click. The **Block element** button is the same, except that a cosmetic rule will be added to the custom rules file making the removal permanent.

URLs of filter lists you want to use are stored in `~/.config/ars/urls` and custom rules in `~/.config/ars/lists/custom`. Both files can be opened from the GUI by clicking on the **Filter lists** and **Custom filters** buttons, respectively.

The **Reload engine** button will restart the adblocking engine applying any changes made to the filter lists and custom filters.  
The **Force update** button is similar, but it also updates every filter list to the latest version.

Filter lists are updated automatically based on the `! Expires:` field.

## Contribute
Feel free to open an issue if you want to report a bug or you have a feature request.
