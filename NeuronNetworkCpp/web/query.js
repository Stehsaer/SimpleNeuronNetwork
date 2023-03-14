function interval_update(doCycle, func) {
    var request = new XMLHttpRequest();

    request.onreadystatechange = function () {
        if (request.readyState == 4) // ready
        {
            var temp;
            if (request.status == 200) // success
            {
                temp = JSON.parse(request.responseText);
            }
            if (request.status >= 400) // error
            {
                console.log("Can't fetch status from server");
            }

            func(temp);

            if (doCycle == true) setTimeout(function () { interval_update(true, func) }, 1000);
        }
    }

    request.open("GET", "/api/query/status");
    request.send(null);
}