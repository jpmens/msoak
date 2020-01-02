# msoak

Subscribe to different MQTT brokers and topics simultaneously, and soak up what they have to offer.

## requirements

You will require:

* [libmosquitto](http://mosquitto.org)
* [libconfig](http://www.hyperrealm.com/libconfig/)
* [Lua](http://www.lua.org)

#### macos

```console
$ brew install lua libconfig
$ make
```

#### freebsd

```console
# pkg install lua53 libconfig mosquitto
$ make
```

#### openbsd

```console
# pkg_add libconfig lua-5.3.5
$ make
```

#### debian

```console
# apt-get install libconfig-dev liblua5.3-dev libmosquitto-dev libbsd-dev
$ make
```

## see also

* [mqttwarn](https://github.com/jpmens/mqttwarn)
