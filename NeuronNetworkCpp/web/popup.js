function hidePopup() {
	mui.overlay('off');
}

function showPopup(caption, message) {

	try {
		hidePopup();
	}
	catch { }

	var content = `
<div id="popup" class="mui-panel mui--z2" style="width: 300px; height: 200px; position: absolute; margin: auto auto; left: 0; right: 0; top: 0; bottom: 0; padding:20px">
	<legend class="mui--text-headline">${caption}</legend>
	<br/>
	<div class="mui--text-body1">
		${message}
	</div>
	<button class="mui-btn mui-btn--primary" style="position:absolute; bottom:20px; right:20px;" onclick="hidePopup()">Got it</button>
</div>
`;

	$("body").append(content);

	mui.overlay('on', document.getElementById("popup"));
}