# msoak

Subscribe to different MQTT brokers and topics simultaneously, and soak up what they have to offer.

## configuration

We use [libconfig](http://www.hyperrealm.com/libconfig/) to read configuration files, each of which has a `servers` array (or list) of settings per server. Please note how each _server_ is in a block of braces.

```
servers = ( { id = "H"
	      topics = [
	     	"owntracks/jpm/+",
		"test",
		"nothing" ]
	    },
	    { id = "L"
	      host = "192.0.2.42",
	      port = 8883
	      user = "jpm"
	      pass = "secrit"
	      cacert = "le-all.pem"
	      showtopic = false
	      topics = [
	     	"owntracks/+/+",
		"nas/home"
	     	]
	    }
	);
```

Token        | Use
------------ | -----------------------------------------------------
`id`         | identifier for the connection, also shown on output. if unset, we generate a numeric `id`
`host`       | hostname. defaults to `localhost`
`port`       | TCP port number for the connection to `host`; defaults to `1883`
`user`	     | username for the MQTT connection
`pass`       | password for the MQTT connection; see `passenv` to omit from config
`retain`     | consume retained messages; default `true`
`passenv`    | name of environment variable (including `$`) from which to obtain `pass`
`cacert`     | path to PEM-encoded CA certificate chain for TLS connections to MQTT broker
`showid`     | `true` or `false`, default `true`; whether to print `id` on output
`showtopic`  | `true` or `false`, default `true`; whether to print topic name on output
`topics`     | array of strings with topic branch names for _msoak_ to subscribe to.
`fmt`        | optional output format, see below


## formatting

By default, the received payload is printed to _stdout_, optionally prefixed by the message `topic` (if `showtopic` is true and preceeded by the connection `id` if `showid` is true.

If `fmt` is set, it contains a string which will be used to print the payload if it's JSON. If the payload does not begin with a brace (`{`) or the payload is not decodable as JSON, the message is ignored (i.e. no output occurs).

After decoding JSON, _msoak_ attempts to substitute the payload elements into the tokens embedded in the `fmt` string.

```
fmt = "{time} {name:%-20s} ({lat:%.4lf}) {station}"
```

The /printf/-like formatting is currently brittle because _msoak_ doesn't do any data conversion. For example, if `lat` below  were published as a string, the resulting output would probably contain 0 at best.

```json
{
  "name": "Jane J.",
  "lat": 48.8460554,
  "lon": 2.277998,
  "station": "Avenue Émile Zola",
  "time": "16:40:30"
}
```

In the above example, with this JSON published to a local server and with the word `hola` published to TMO, we get (the `888` and `tmo` prefixing the lines are the respective `id`s of the connections):

```
888 16:40:30 Jane J.              (48.8461) Avenue Émile Zola
tmo home/special hola
```


## requirements

You will require:

* [libmosquitto](http://mosquitto.org)
* [libconfig](http://www.hyperrealm.com/libconfig/)

## bugs

Yes, I'm sure.

What's with the strange name? Just as I started on _msoak_, I learned of `sponge(1)` and  loved the name. The rest, is history.
