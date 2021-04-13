//const apiBaseUrl = "http://loc/alhost:8080";

const apiBaseUrl = "";

// Apex.tooltip.x.format = 'HH:mm:ss';

function apiUrl(path) {
    if (apiBaseUrl !== '')
        return new URL(path, apiBaseUrl).href;
    return path;
}

export {apiUrl};
