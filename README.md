# msoak

Subscribe to different MQTT brokers and topics simultaneously, and soak up what they have to offer.

## configuration

We use [libconfig](http://www.hyperrealm.com/libconfig/) to read configuration files, each of which has a `servers` array (or list) of settings per server. Please note how each _server_ is in a block of braces.

```
# this is a comment

verbose = true
luascript = "my.lua"

servers = (
	    { id = "888"
	      port = 1883,
	      showid = true
	      showtopic = false
	      fmt = "station"
	      topics = [
	     	"tests/sponge/#",
	     	]
	    },
	    { id = "tmo"
	      host = "test.mosquitto.org",
	      showid = true
	      showtopic = true
	      topics = [
	     	"home/special",
		"nothome/dont",
	     	]
	    },
	);
```

The configuration file consits of global variables and a block of servers.

### globals

Token        | Use
------------ | -----------------------------------------------------
`luascript`  | path to the Lua script to use
`verbose`    | `true` (default) or `false`


### servers

Each element of the `server` array defines one server:

Token        | Use
------------ | -----------------------------------------------------
`id`         | identifier for the connection, also shown on output. if unset, we generate a numeric `id`
`host`       | hostname. defaults to `localhost`
`port`       | TCP port number for the connection to `host`; defaults to `1883`
`clientid`   | MQTT clientId to use (default: program name)
`user`	     | username for the MQTT connection
`pass`       | password for the MQTT connection; see `passenv` to omit from config
`retain`     | consume retained messages; default `true`
`passenv`    | name of environment variable (including `$`) from which to obtain `pass`
`cacert`     | path to PEM-encoded CA certificate chain for TLS connections to MQTT broker
`showid`     | `true` or `false`, default `true`; whether to print `id` on output
`showtopic`  | `true` or `false`, default `true`; whether to print topic name on output
`topics`     | array of strings with topic branch names for _msoak_ to subscribe to.
`fmt`        | optional name of Lua formatting function (see below)


## formatting

By default, the received payload is printed to _stdout_, optionally prefixed by the message `topic` (if `showtopic` is true and preceeded by the connection `id` if `showid` is true.

If `fmt` is set, it contains a string with the name of a Lua function from the Lua script. this function is used to format the payload _msoak_ receives; the return value of the function is printed.

If _msoak_ can decode the payload into JSON (i.e. it begins with a brace (`{`) and is JSON), it will invoke the Lua function to obtain output.

If the above configuration, the `luascript` is loaded from `my.lua` and contains the following:

```lua
function init()
	if msoak.verbose then 
		msoak.log(msoak.luascript .. " starting at " .. msoak.strftime("%FT%TZ"))
	end
end

function station(topic, _type, d)
	
	s = string.format("%s %-20s (%.4f) %s  -> now=%s",
		d.time, d.name, d.lat, d.station,
		msoak.strftime("%TZ"))
	return s

end

function exit()
	msoak.log("Hasta la vista, baby!")
end

```

The optional `init()` and `exit()` functions are invoked when _msoak_ begins and ends respectively. The `station` function is invoked for each message _msoak_ receives because the configuration states

```
fmt = "station"
```

```json
{
  "name": "Jane J.",
  "lat": 48.8460554,
  "lon": 2.277998,
  "station": "Avenue Émile Zola",
  "time": "14:41:13"
}
```

In the above example, with this JSON published to a local server and with the word `hola` published to TMO, we get (the `888` and `tmo` prefixing the lines are the respective `id`s of the connections):

```
888 14:41:13 Jane J.              (48.8461) Avenue Émile Zola  -> now=12:43:28Z
tmo home/special hola
```


## requirements

You will require:

* [libmosquitto](http://mosquitto.org)
* [libconfig](http://www.hyperrealm.com/libconfig/)
* [Lua](http://www.lua.org)

### macos

```
brew install lua
brew install libconfig
```

### debian

```
apt-get install libconfig-dev liblua5.3-dev
```

## bugs

Yes, I'm sure.

What's with the strange name? Just as I started on _msoak_, I learned of `sponge(1)` and  loved the name. The rest, is history.
