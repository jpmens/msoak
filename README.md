# msoak

Subscribe to different MQTT brokers and topics simultaneously, and soak up what they have to offer.

## startup

Launch _msoak_ with the example configuration file:

```console
$ msoak example.config
```

Then publish a message:

```bash
mosquitto_pub -t tests/sponge/1 -m "$(jo name=Jane number=42)"
```

you should see the payload as formatted by Lua. The word `loc` at start of output is the _id_ of the connection as specified in `example.config`. ([jo](https://github.com/jpmens/jo) is a utility to easily produce JSON output in a shell, and it may well be available as a package for your system of choice.)

```
loc tests/sponge/1 Hello Jane -> now=18:12:50Z
```

## requirements

### running

To test msoak, you will need a source of MQTT test traffic, This could be a test server or a production server that you run, or one of the [public test servers](https://github.com/mqtt/mqtt.github.io/wiki/public_brokers). The example configuration assumes that you have a test server running on localhost; edit this to suit.

### building

In order to build `msoak` you will require:

* [libmosquitto](http://mosquitto.org)
* [libconfig](http://www.hyperrealm.com/libconfig/)
* [Lua](http://www.lua.org)

Edit the `Makefile` to select the system you are building for.

#### macos

```console
$ brew install lua libconfig mosquitto
$ make
```

#### freebsd

```console
# pkg install lua53 libconfig mosquitto
$ make
```

#### openbsd

```console
# pkg_add libconfig lua-5.3.5 mosquitto
$ make
```

#### debian, ubuntu

```console
# apt-get install libconfig-dev liblua5.3-dev libmosquitto-dev libbsd-dev
$ make
```

### alpine

```console
# apk add build-base libconfig-dev mosquitto-dev lua5.3-dev libbsd-dev
$ make
```


## see also

* [mqttwarn](https://github.com/jpmens/mqttwarn)
