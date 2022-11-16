var jmuxer;
var socket;
var total = 0;
var counts = 0;
const socket_url = "ws://localhost:3001/videostream";

function connect() {
    if (!socket || socket.readyState === 3) {
        socket = new WebSocket(socket_url);
        socket.addEventListener('message', (event) => {
            let payload = event.data;
            total += new TextEncoder().encode(payload).length;
            counts++;
            let { data } = JSON.parse(payload);
            
            jmuxer.feed({
                video: new Uint8Array(data)  // decode from base64
            });
        });
        socket.addEventListener('error', function(e) {
            console.log('Socket Error');
         });
    }
}

function disconnect() {
    if (socket.readyState === 1) {
        socket.close(Math.round(total / counts));
    }
    console.log();
}

window.onload = () => {
    jmuxer = new JMuxer({
        node: 'player',
        mode: 'video',
        flusingTime: 0,
        onError: function(data) {
            console.log('Buffer error encountered', data);
        },
        onMissingVideoFrames: function (data) {
            console.log('Video frames missing', data);
        },
        clearBuffer: false,
        fps: 20,
        /* maxDelay: 0, */
        /* readFpsFromTrack: true, */
        debug: false
    });
};