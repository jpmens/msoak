# msoak

Subscribe to different MQTT brokers and topics simultaneously, and soak up what they have to offer.

## requirements

You will require:

* [libmosquitto](http://mosquitto.org)
* [libconfig](http://www.hyperrealm.com/libconfig/)
* [Lua](http://www.lua.org)

#### macos

```
brew install lua
brew install libconfig
```

#### freebsd

```
pkg install lua53 libconfig mosquitto
```

#### debian

```
apt-get install libconfig-dev liblua5.2-dev # or liblua5.3-dev
```

## see also

* [mqttwarn](https://github.com/jpmens/mqttwarn)
