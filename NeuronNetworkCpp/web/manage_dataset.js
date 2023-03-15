var list;

var label_name = null;
var image_name = null;

function do_table_refresh() {
    $("#dataset-list-table>tbody").empty();

    list["list"].forEach(element => {
        var name = element["name"];
        var size = element["size"] / 1048576;
        size = size.toFixed(3);

        var clas = name == label_name || name == image_name ? "mui-btn--accent" : "mui-btn--primary";
        var txt = name == label_name || name == image_name ? "Unselect" : "Select";

        var disb = "";
        if (name != label_name && name.endsWith(".labels") && label_name != null) disb = "disabled";
        if (name != image_name && name.endsWith(".images") && image_name != null) disb = "disabled";

        $("#dataset-list-table>tbody").append(`
                        <tr >
                            <td>${name}</td>
                            <td>${size} MB</td>
                            <td><button name="${name}" class="mui-btn dataset-select mui--z1 ${clas}" ${disb}>${txt}</button></td>
                        </tr>
                    `);
    });
}

function delete_slot(slot) {

    var request = new XMLHttpRequest();

    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            if (request.status == 200) {
                showPopup("Processing", "Request processing, please wait");
            }
            if (request.status == 403) {
                showPopup("Server Busy", "Server is busy at the moment, please try again later");
            }
        }
    }

    request.open("POST", `/api/command/clear_slot?slot=${slot}`);
    request.send(null);
}

function refresh() {
    var request = new XMLHttpRequest();

    request.onreadystatechange = function () {
        if (request.readyState == 4) // ready
        {
            if (request.status == 200) // success
            {
                list = JSON.parse(request.responseText);

                do_table_refresh();
            }
            if (request.status >= 400) // error
            {
                console.log("Unable to fetch status from server");
            }

        }
    }

    request.open("GET", "/api/query/dataset_list");
    request.send(null);
}

refresh();

$(document).on("click", ".dataset-select", function () {
    var name = $(this).attr("name");

    if (name == label_name || name == image_name) {
        if (name.endsWith(".labels")) label_name = null;
        if (name.endsWith(".images")) image_name = null;
    }
    else {
        if (name.endsWith(".labels")) label_name = name;
        if (name.endsWith(".images")) image_name = name;
    }

    do_table_refresh();
    console.log(`${image_name},${label_name}`);
});

var prev_slot_count;

function doUpdate(status_json) {

    var max_dataset_num = status_json["max_dataset_num"];

    if (prev_slot_count != max_dataset_num) {
        $("#slot-selection").empty();
        for (var num = 0; num < max_dataset_num; num++) {
            $("#slot-selection").append(`<option value="${num}">Slot ${num}</option>`);
        }
    }

    prev_slot_count = max_dataset_num;

    // list
    $("#dataset-table>tbody").empty();

    status_json["datasets_info"].forEach(element => {
        var name = element["name"];
        if (name == "") name = "--";

        var data_height = element["data_height"];
        var data_width = element["data_width"];
        var data_count = element["data_count"];
        var data_space = Math.round(data_height * data_width * data_count * 4 / 1048576, 2);

        $("#dataset-table>tbody").
            append(`<tr><td>${name}</td>
<td>(${data_height}, ${data_width})</td>
<td>${data_count}</td>
<td>${data_space} MB
<td><button class="mui-btn mui-btn--primary mui--z1" onclick="delete_slot(${element["index"]})" ${data_count<=0?"disabled":""}>Clear</button>
</tr>`)
    });

    get_success(status_json);
}

interval_update(true, doUpdate);

function LoadDataset() {

    if (label_name != null && image_name != null) {
        var slot = $("#slot-selection").val();
        var name = $("#name-input").val();

        //console.log(`${slot},${name}`);

        if (name == "") {
            showPopup("Error", "Please specify a name for the dataset slot");
            return;
        }

        var request = new XMLHttpRequest();

        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                if (request.status == 200) {
                    showPopup("Processing", "Request processing, please wait");
                }
                if (request.status == 403) {
                    showPopup("Server Busy", "Server is busy at the moment, please try again later");
                }
            }
        }

        request.onerror = function (e) {
            var statusText = request.statusText;
            showPopup("Network Error", `Network error, try again later.<br/>Message: ${statusText}`);
        }

        request.open("POST", `/api/command/load_dataset?image=${image_name}&label=${label_name}&name=${name}&slot=${slot}`);
        request.send(null);

        image_name = null;
        label_name = null;
        do_table_refresh();
    }
    else {
        showPopup("Error", "Please select datasets to load");
    }
}