var jmixer;
var socket;
const socket_url = "ws://localhost:3001/videostream";

function connect() {
    socket = new WebSocket(socket_url);
    socket.addEventListener('message', (event) => {
        console.log(event.data);
        /*
        jmuxer.feed({
            video: new Uint8Array(atob(event.data))  // decode from base64
        });
        */
    });
}

function disconnect() {
    socket.close();
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