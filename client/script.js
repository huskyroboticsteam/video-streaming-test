var jmixer;
var socket;
const socket_url = "ws://localhost:3001/videostream";

function connect() {
    if (!socket || socket.readyState === 3) {
        socket = new WebSocket(socket_url);
        socket.addEventListener('message', (event) => {
            let { data } = JSON.parse(event.data);
            console.log(data);
            /*
            jmuxer.feed({
                video: new Uint8Array(atob(data))  // decode from base64
            });
            */
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
        /* maxDelay: 0, */
        /* readFpsFromTrack: true, */
        debug: true
    });
};