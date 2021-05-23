import { apiUrl } from "./config";

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
			body: new URLSearchParams({ value: 0 }),
		}).then((response) => console.log(response));
		// const body = new FormData();
		// body.append("value", 0);
		// fetch(apiUrl("/weight"), {
		// 	method: "POST",
		// 	body: body,
		// }).then((response) => console.log(response));
	}
}

const apiClient = new APIClient();
window.apiClient = apiClient;

export default apiClient;
