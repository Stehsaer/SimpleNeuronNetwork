var prev_slot;

function refresh_model_status(status_json) {
	//console.log(status_json);

	var model_status_text = $("#model-status");
	var model_parameter_text = $("#model-parameter");

	if (status_json["network_info"] == null) {
		model_status_text.text("Empty");
		model_parameter_text.text("Empty");
	}
	else {
		model_status_text.text("Loaded");
		model_parameter_text.text(`${JSON.stringify(status_json["network_info"])}`);
	}

	if (JSON.stringify(status_json["datasets_info"]) != JSON.stringify(prev_slot)) {

		var max_dataset_num = status_json["max_dataset_num"];
		$("#slot-input").empty();

		for (var num = 0; num < max_dataset_num; num++) {
			var d = status_json["datasets_info"][num];
			$("#slot-input").append(`<option value="${num}">${d["name"]} (Slot ${num}) ${d["data_count"] <= 0 ? "(Empty)":""}</option>`);
		}

	} prev_slot = status_json["datasets_info"];

	if (status_json["status"] > 0 && status_json["current_task"] == 3) {
		$("#train-detail-block").show();

		$("#progress-text").text(`${Math.round(status_json["progress"] * 100, 2)}%`);
		$("#progress-indicator").val(status_json["progress"]);
		$("#progress-detail").text(status_json["status_text"]);

	}
	else {
		$("#train-detail-block").hide();
    }

	get_success(status_json);
}

interval_update(true, refresh_model_status);

function create_model() {
	var inNeuronCount = $("#in-input").val();
	var outNeuronCount = $("#out-input").val();
	var layerCount = $("#layer-input").val();
	var layerNeuronCount = $("#count-input").val();
	var activateFunc = $("#func-input").val();

	if (inNeuronCount == '' || outNeuronCount == '' || layerCount == '' || layerNeuronCount == '') {
		showPopup("Missing Parameter", "Please fill in all the parameters required!");
		return;
    }

	var request = new XMLHttpRequest();

	request.onreadystatechange = function () {
		if (request.readyState == 4) {
			if (request.status == 200) {
				showPopup("Processing", "Creating model, please wait");
			}
			if (request.status == 403) {
				showPopup("Server Busy", "Server is busy at the moment, please try again later");
			}
		}
	}

	request.onerror = function () {
		showPopup("Error!", "An error occured when trying to reach the server");
	}

	request.open("POST", `/api/command/create_model?in=${inNeuronCount}&out=${outNeuronCount}&layer=${layerCount}&count=${layerNeuronCount}&func=${activateFunc}`);
	request.send(null);
}

function start_training() {
	var slot = $("#slot-input").val();
	var learningRate = $("#learning-rate-input").val();
	var threshold = $("#threshold-input").val();
	var maxIter = $("#maxiter-input").val();

	if (learningRate == '' || threshold == '' || maxIter == '') {
		showPopup("Missing Parameter", "Please fill in all the parameters required!");
		return;
	}

	var request = new XMLHttpRequest();

	request.onreadystatechange = function () {
		if (request.readyState == 4) {
			if (request.status == 200) {
				showPopup("Processing", "Training model, please wait");
			}
			if (request.status == 403) {
				showPopup("Server Busy", "Server is busy at the moment, please try again later");
			}
		}
	}

	request.open("POST", `/api/command/train_model?slot=${slot}&learningRate=${learningRate}&maxIter=${maxIter}&threshold=${threshold}`);
	request.send(null);
}