#!/bin/bash

# Setup XDG default paths
DEFAULT_XDG_CONF_HOME="$HOME/.config"
DEFAULT_XDG_DATA_HOME="$HOME/.local/share"
export XDG_CONFIG_HOME="${XDG_CONFIG_HOME:-$DEFAULT_XDG_CONF_HOME}"
export XDG_DATA_HOME="${XDG_DATA_HOME:-$DEFAULT_XDG_DATA_HOME}"
THIS_DIR="$(dirname "$(realpath "$BASH_SOURCE")")"

sudo true

# Install dependencies
sudo apt-get install -y libssl-dev libgflags-dev libgoogle-glog-dev libboost-all-dev

make

# cleanup old service name (we call it just "polaris")
sudo rm -rf $XDG_DATA_HOME/polaris-client-mavlink &>/dev/null

# Setup project directory
cp $THIS_DIR/build/polaris-client-mavlink ~/.local/bin
mkdir -p $XDG_DATA_HOME/polaris
cp $THIS_DIR/config.toml $XDG_DATA_HOME/polaris/

# Modify config file if ENV variables are set
CONFIG_FILE="$XDG_DATA_HOME/polaris/config.toml"

if [ -n "$POLARIS_API_KEY" ]; then
	echo "Setting polaris_api_key to: $POLARIS_API_KEY"
	sed -i "s/^polaris_api_key = \".*\"/polaris_api_key = \"$POLARIS_API_KEY\"/" "$CONFIG_FILE"
fi
