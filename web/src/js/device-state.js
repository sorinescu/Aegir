import { makeAutoObservable, autorun, runInAction } from "mobx";
import localStore from "store2";
import apiClient from "./api-client";
import { apiUrl } from "./config";

function computeLocalTimeFromDevice(localTime, deviceCrtTime, deviceTimePoint) {
	let timeDelta = localTime.getTime() - deviceCrtTime;
	return deviceTimePoint + timeDelta;
}

export class DeviceState {
	temperature = "N/A";
	temperatureHistory = [];
	weight = "N/A";
	weightHistory = [];
	recipe = localStore.get("recipe");

	constructor() {
		makeAutoObservable(this);
	}

	fetchTemperatureHistory(minutes) {
		apiClient.fetchTemperatureHistory(minutes).then((data) =>
			runInAction(() => {
				let localTime = new Date();
				deviceState.temperatureHistory = data.values.map((item) => [
					computeLocalTimeFromDevice(localTime, data.ts, item.ts),
					item.value,
				]);
			})
		);
	}

	fetchTemperature() {
		apiClient.fetchTemperature().then((data) =>
			runInAction(() => {
				deviceState.temperature = data.value;
			})
		);
	}

	fetchWeightHistory(minutes) {
		apiClient.fetchWeightHistory(minutes).then((data) =>
			runInAction(() => {
				let localTime = new Date();
				deviceState.weightHistory = data.values.map((item) => [
					computeLocalTimeFromDevice(localTime, data.ts, item.ts),
					item.value,
				]);
			})
		);
	}

	fetchWeight() {
		apiClient.fetchWeight().then((data) =>
			runInAction(() => {
				deviceState.weight = data.value;
			})
		);
	}
}
const deviceState = new DeviceState();

(function () {
	// Initial fetch and load every 10s
	deviceState.fetchTemperature();
	deviceState.fetchTemperatureHistory(15);
	setInterval(() => {
		deviceState.fetchTemperatureHistory(15);
	}, 10000);

	// Initial fetch and load every 10s
	deviceState.fetchWeight();
	deviceState.fetchWeightHistory(15);
	setInterval(() => {
		deviceState.fetchWeightHistory(15);
	}, 10000);

	if (!!window.EventSource) {
		var source = new EventSource(apiUrl("/events"));

		source.addEventListener(
			"temperature",
			(e) => {
				var data = JSON.parse(e.data);
				runInAction(() => {
					deviceState.temperature = data.value;
				});

				// updateTime(data.ts);

				// let time = new Date();
				// let update = {
				//     x: [
				//         [time]
				//     ],
				//     y: [
				//         [data.value]
				//     ]
				// };

				// var olderTime = time.setHours(time.getHours() - 1);
				// var futureTime = time.setHours(time.getHours() + 1);

				// var hourView = {
				//     xaxis: {
				//         type: 'date',
				//         range: [olderTime, futureTime]
				//     }
				// };
				// Plotly.relayout('tempHistory', hourView);
				// Plotly.extendTraces('tempHistory', update, [0])
			},
			false
		);

		source.addEventListener(
			"weight",
			(e) => {
				var data = JSON.parse(e.data);
				runInAction(() => {
					deviceState.weight = data.value;
				});
			},
			false
		);

		source.addEventListener(
			"open",
			(e) => {
				// Connection was opened.
				console.log("Device stream connection opened");
			},
			false
		);

		source.addEventListener(
			"error",
			(e) => {
				if (e.readyState == EventSource.CLOSED) {
					console.log("Device stream connection closed");
				}
			},
			false
		);
	} else {
		// Load temperature every 10 seconds
		setInterval(deviceState.fetchTemperature, 10000);

		// Load weight every second
		setInterval(deviceState.fetchWeight, 1000);
	}
})();

window.deviceState = deviceState;
window.mobxAutorun = autorun;

export default deviceState;
