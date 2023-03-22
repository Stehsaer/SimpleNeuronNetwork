const progressElement = `<div class="mui-panel display-block mui--z3" id="detail-block" 
         style="width:300px; position:fixed; bottom:20px; left:50%; transform: translate(-50%, 0); background-color:#ffffff80; backdrop-filter:blur(8px) saturate(0.7)" hidden>
        <legend class="mui--text-headline">Progress</legend>
        <br />

        <strong><span id="progress-text" class="mui--text-body1"></span></strong>
        <progress value="0.5" id="progress-indicator" style="margin-left:10px; display:inline; width:200px;"></progress><br />
        <span id="progress-detail"></span>
    </div>`;

function init_progress_display() {
    if ($("#detail-block").length == 0) {
        $("body").append(progressElement);
    }
}

function update_status(status) {
	if (status["status"] > 0) {
		$("#detail-block").show();

		var progress = status["progress"];

		if (progress < 0) {
			$("#progress-indicator").removeAttr("value");
			$("#progress-text").hide();
		}
		else {
			$("#progress-indicator").val(status["progress"]);
			$("#progress-text").show();
			$("#progress-text").text(`${Math.round(progress * 100, 2)}%`);
		}
		$("#progress-detail").text(status["status_text"]);

	}
	else {
		$("#detail-block").hide();
	}
}