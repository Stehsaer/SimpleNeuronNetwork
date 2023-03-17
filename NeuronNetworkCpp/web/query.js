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

            if (doCycle == true) setTimeout(function () { interval_update(true, func) }, 500);
        }
    }

    request.onerror = function () {
        console.log("Query error!");
        if (doCycle == true) setTimeout(function () { interval_update(true, func) }, 500);
    }

    request.open("GET", "/api/query/status");
    request.send(null);
}

function get_success(status) {
    if (status["status"] == 3) {
        // clear success flag
        var request = new XMLHttpRequest();
        request.open("POST", "/api/command/clear_status");
        request.send(null);

        if (status["success"] == 1) {
            showPopup("Success", `Done with success! <br/><strong>Message:</strong> ${status["status_text"]}`);
        }
        else {
            showPopup("Fail", `An error occured. <br/><strong>Message:</strong> ${status["status_text"]}`);
        }
    }
}

function get_success_callback(status, callback) {
    if (status["status"] == 3) {
        // clear success flag
        var request = new XMLHttpRequest();
        request.open("POST", "/api/command/clear_status");
        request.send(null);

        if (status["success"] == 1) {
            showPopup("Success", `Done with success! <br/><strong>Message:</strong> ${status["status_text"]}`);
        }
        else {
            showPopup("Fail", `An error occured. <br/><strong>Message:</strong> ${status["status_text"]}`);
        }

        callback();
    }
}