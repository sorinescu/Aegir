export class APIClient {
    fetchTemperatureHistory(minutes) {
        return fetch(
            apiUrl(`/temperature/history?minutes=${minutes}`)
        ).then((response) => response.json());
    }

    fetchTemperature() {
        return fetch(apiUrl("/temperature")).then((response) => response.text());
    }

    fetchWeightHistory(minutes) {
        return fetch(
            apiUrl(`/weight/history?minutes=${minutes}`)
        ).then((response) => response.json());
    }

    fetchWeight() {
        return fetch(apiUrl("/weight")).then((response) => response.text());
    }

    tareWeight() {
        fetch(apiUrl("/weight"), {
            method: "POST",
            body: new URLSearchParams({ value: "0" }),
        }).then((response) => console.log(response));
        // const body = new FormData();
        // body.append("value", 0);
        // fetch(apiUrl("/weight"), {
        // 	method: "POST",
        // 	body: body,
        // }).then((response) => console.log(response));
    }

    calibrateWeight(knownWeight) {
        fetch(apiUrl("/weight/calibrate"), {
            method: "POST",
            body: new URLSearchParams({ value: knownWeight.toString() }),
        }).then((response) => console.log(response));
    }
}

function apiUrl(path) {
    return process.env.VUE_APP_API_BASE_URL + path
}

const apiClient = new APIClient();
window.apiClient = apiClient;

export default apiClient;
