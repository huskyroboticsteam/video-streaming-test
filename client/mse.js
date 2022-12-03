(() => {
    const socket_url = "ws://localhost:3001/videostream";
    var mime = 'video/mp4; codecs="avc1.4D0029"';
    if (!MediaSource.isTypeSupported(mime)) {
        alert('Unsupported mime type');
        return;
    }
    var buffer;
    var socket;
    var video = document.getElementById('player');
    var mediaSource = new MediaSource();
    video.src = URL.createObjectURL(mediaSource);
    mediaSource.addEventListener('sourceopen', function(e) {
        console.log('sourceopen: ' + mediaSource.readyState);
        buffer = mediaSource.addSourceBuffer(mime);
        buffer.mode = 'sequence';
        buffer.addEventListener('update', function(e) {
            console.log('update');
        });
        buffer.addEventListener('updateend', function(e) {
            console.log('updateend');
            video.play();
        });

        socket = new WebSocket(socket_url);
        socket.addEventListener('message', function (event) {
            let payload = event.data;
            let { data } = JSON.parse(payload);
            if (!buffer.updating) {
                buffer.appendBuffer(new Uint8Array(data));
            }
        });

    }, false);
})();