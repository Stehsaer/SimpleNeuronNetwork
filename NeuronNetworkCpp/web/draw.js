var canvas;
var ctx;

var penClick = false;
var startAxisX = 0;
var startAxisY = 0;
var penColor = "white";
var penWidth = 2;
var scaleRatio;

function clearCanvas(size) {
    ctx.fillStyle = "#000000";
    ctx.fillRect(0, 0, size, size);
}

function initCanvas(resolution, actualSize) {
    canvas = document.getElementById('canvas');
    ctx = canvas.getContext("2d");

    clearCanvas(resolution);

    canvas.addEventListener("mousemove", drawing, true);
    canvas.addEventListener("mousedown", penDown, false);
    canvas.addEventListener("mouseup", penUp, false);

    scaleRatio = resolution / actualSize;
    penWidth = resolution / 15;
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
    ctx.moveTo((startAxisX - pos.left) * scaleRatio, (startAxisY - pos.top) * scaleRatio);//moveTo(x,y) 定义线条开始坐标
    ctx.lineTo((stopAxisX - pos.left) * scaleRatio, (stopAxisY - pos.top) * scaleRatio);//lineTo(x,y) 定义线条结束坐标
    ctx.strokeStyle = penColor;
    ctx.lineWidth = penWidth;
    ctx.lineCap = "round";
    ctx.stroke();
    startAxisX = stopAxisX;
    startAxisY = stopAxisY;
}