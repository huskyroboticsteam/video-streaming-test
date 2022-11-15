var jmuxer;
var socket;
const socket_url = "ws://localhost:3001/videostream";

function connect() {
    if (!socket || socket.readyState === 3) {
        socket = new WebSocket(socket_url);
        socket.addEventListener('message', (event) => {
            let { data } = JSON.parse(event.data);
            
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
        socket.close();
    }
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
        debug: true
    });
};