function init()
    if msoak.verbose then 
        msoak.log(msoak.luascript .. " starting up")
    end
end

function greet(topic, _type, d)
    s = string.format("Hello %s -> now=%s", d.name, 
        msoak.strftime("%TZ"))
    return s
end

function exit()
    msoak.log("Hasta la vista, baby!")
end

