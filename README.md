# blocKit
> WebKitGTK adblock extension with Brave's Rust-based adblock engine for backend.

## Features
* Network and cosmetic filtering
* Simple GUI
* Automatic filter lists updating
* Element picker/zapper

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
3. Link the library located at `/usr/local/lib/blockit.so` with the WebKitGTK based browser you want to use:
```
sudo ln -s /usr/local/lib/blockit.so /usr/lib/example-browser
```
The target folder is different for each browser. The locations can be found at [wyebadblock's Github page](https://github.com/jun7/wyebadblock#addition-for-other-webkit2gtk-browsers).

The server should start automatically when you open your browser. If not, try starting it manually:
```
adblock-rust-server
```

Filter lists are located in `~/.config/ars/lists`. By default the folder doesn't exist. You can use lists like [easylist](https://easylist.to/easylist/easylist.txt) or [easyprivacy](https://easylist.to/easylist/easyprivacy.txt). Just put them in the directory and restart the server.
