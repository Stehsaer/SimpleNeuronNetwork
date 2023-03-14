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
