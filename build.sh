#!/usr/bin/env bash

export MYKATEPLUGINPATH=/home/mte90/Desktop/Prog/kate-ai/
mkdir -p build && cd build
cmake .. -DKDE_INSTALL_PLUGINDIR=$MYKATEPLUGINPATH
make
make install

export QT_PLUGIN_PATH=$MYKATEPLUGINPATH:$QT_PLUGIN_PATH
kate --startanon
