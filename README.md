# MSOAK

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
`host`       | hostname. defaults to `localhost` if unset
`port`       | TCP port number for the connection to `host`; defaults to `1883`
`user`	     | username for the MQTT connection
`pass`       | password for the MQTT connection; see `passenv` to omit from config
`passenv`    | name of environment variable (including `$`) from which to obtain `pass`
`cacert`     | path to PEM-encoded CA certificate chain for TLS connections to MQTT broker
`showtopics` | `true` or `false`, default `true`; whether to print topic name on output
`topics`     | array of strings with topic branch names for _msoak_ to subscribe to.

## requirements

You will require:

* [libmosquitto](http://mosquitto.org)
* [libconfig](http://www.hyperrealm.com/libconfig/)
