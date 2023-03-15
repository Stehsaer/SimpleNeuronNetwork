var canvas;
var ctx;

var penClick = false;
var startAxisX = 0;
var startAxisY = 0;
var penColor = "white";
var penWidth = 2;
var scaleRatio;

function clearCanvas() {
    ctx.fillStyle = "#000000";
    ctx.fillRect(0, 0, 28, 28);
}

function initCanvas(actualSize) {
    canvas = document.getElementById('canvas');
    ctx = canvas.getContext("2d");

    clearCanvas();

    canvas.addEventListener("mousemove", drawing, true);
    canvas.addEventListener("mousedown", penDown, false);
    canvas.addEventListener("mouseup", penUp, false);

    scaleRatio = 28 / actualSize;
}

function penDown(event) {
    penClick = true;
    startAxisX = event.clientX;
    startAxisY = event.clientY;
}

function penUp() {
    penClick = false;
}

function drawing(event) {
    if (!penClick)
        return;
    var stopAxisX = event.clientX;
    var stopAxisY = event.clientY;

    var pos = canvas.getBoundingClientRect();

    ctx.beginPath();
    ctx.moveTo((startAxisX - pos.left) * scaleRatio, (startAxisY - pos.top) * scaleRatio);//moveTo(x,y) ����������ʼ����
    ctx.lineTo((stopAxisX - pos.left) * scaleRatio, (stopAxisY - pos.top) * scaleRatio);//lineTo(x,y) ����������������
    ctx.strokeStyle = penColor;
    ctx.lineWidth = penWidth;
    ctx.lineCap = "round";
    ctx.stroke();
    startAxisX = stopAxisX;
    startAxisY = stopAxisY;
}