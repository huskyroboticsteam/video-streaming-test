const url = "http://localhost:8000/bgr";
const img_tag = "videosource";

function connect() {
    document.getElementById(img_tag).src = url;
}

function disconnect() {
    document.getElementById(img_tag).src = "";
}