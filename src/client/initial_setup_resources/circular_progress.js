function CircularProgressBar(canvas, color) {
    const ROTATION_SPEED = 3;
    const SIZE_SPEED = 2;
    const SIZE_COLLAPSE_SPEED = 1;
    const MIN_SIZE = 0.1 * Math.PI;
    const MAX_SIZE = 1.5 * Math.PI;

    this.element = canvas;

    var context = canvas.getContext("2d");
    var prevTime = performance.now();
    var rot = Math.PI*3/2;
    var size = 0;
    var sizeCollapse = false;
    var drawingEnabled = false;
    var hasQueuedDraw = false;

    this.setDrawingEnabled = function(enabled) {
        drawingEnabled = enabled;
        if (enabled && !hasQueuedDraw) {
            hasQueuedDraw = true;
            requestAnimationFrame(draw);
        }
    };

    function draw() {
        var now = performance.now();
        var delta = (now - prevTime) / 1000;
        prevTime = now;

        context.clearRect(0, 0, canvas.width, canvas.height);
        context.strokeStyle = color;
        context.lineWidth = 4;
        context.beginPath();
        rot += delta * ROTATION_SPEED;
        var easedSize;
        var off = 0;
        if (sizeCollapse) {
            size = Math.max(size - delta * SIZE_COLLAPSE_SPEED, 0);
            easedSize = size * size;
            if (size <= 0) {
                size = 0;
                sizeCollapse = false;
                rot += MAX_SIZE - MIN_SIZE;
            } else {
                off = (MAX_SIZE - MIN_SIZE) * (1 - easedSize);
            }
        } else {
            size = size + delta * SIZE_SPEED;
            easedSize = (1 - Math.cos(Math.PI * size)) / 2;
            if (size >= 1) {
                size = 1;
                sizeCollapse = true;
            }
        }
        context.arc(canvas.width / 2, canvas.height / 2, 16, rot + off, rot + off + MIN_SIZE + (MAX_SIZE - MIN_SIZE) * easedSize, false);
        context.stroke();
        if (drawingEnabled) {
            hasQueuedDraw = true;
            requestAnimationFrame(draw);
        } else {
            hasQueuedDraw = false;
        }
    }
}