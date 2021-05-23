// Apex.tooltip.x.format = 'HH:mm:ss';

function apiUrl(path) {
    let apiPath = '/api' + path;
    // Debug support - serve API from Python simulator
    if (document.location.host.includes('localhost') || document.location.host.startsWith('127.'))
        return new URL(apiPath, 'http://localhost:8080').href;
    return apiPath;
}

export {apiUrl};
