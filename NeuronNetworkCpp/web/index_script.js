var server_status;
var server_status_text = ["Idle", "Queried", "Working", "Done"];

function update_elements(server_status_json)
{
	server_status = server_status_json;

	$("#server-status-text").html(server_status_text[server_status_json["status"]]);

	if(server_status_json["network_info"] == null)
	{
		$("#server-network-text").text("Not loaded");
	}
	else
	{
		$("#server-network-text").text("Stand by");
	}

	$("#dataset-table>tbody").empty();

	server_status_json["datasets_info"].forEach(element => 
		{
		var name = element["name"];
		if(name == "") name = "--";

		var data_height = element["data_height"];
		var data_width = element["data_width"];
		var data_count = element["data_count"];
		var data_space = Math.round(data_height*data_width*data_count*4 / 1048576, 2);

		$("#dataset-table>tbody").
			append(`<tr><td>${name}</td><td>(${data_height}, ${data_width})</td><td>${data_count}</td><td>${data_space} MB</tr>`)
	});

	get_success(server_status_json);
}

interval_update(true, update_elements);