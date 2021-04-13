import { apiUrl } from "./config";
import { makeAutoObservable, autorun, runInAction } from "mobx";
import localStore from "store2";

function computeLocalTimeFromDevice(localTime, deviceCrtTime, deviceTimePoint) {
	let timeDelta = localTime.getTime() - deviceCrtTime;
	return deviceTimePoint + timeDelta;
}

export class DeviceState {
	temperature = "N/A";
	temperatureHistory15Min = [];
	weight = "N/A";
	recipe = localStore.get("recipe");

	constructor() {
		makeAutoObservable(this);
	}

	fetchTemperatureHistory15Min() {
		fetch(apiUrl("/temperature/history?minutes=15"))
			.then(response => response.json())
			.then(data =>
				runInAction(() => {
					let localTime = new Date();
					deviceState.temperatureHistory15Min = data.values.map(item => [
						computeLocalTimeFromDevice(localTime, data.ts, item.ts),
						item.value,
					]);
				})
			);
	}

	fetchTemperature() {
		fetch(apiUrl("/temperature"))
			.then(response => response.text())
			.then(data =>
				runInAction(() => {
					deviceState.temperature = data.value;
				})
			);
	}

    fetchWeight() {
		fetch(apiUrl("/weight"))
			.then(response => response.text())
			.then(data =>
				runInAction(() => {
					deviceState.weight = data.value;
				})
			);
	}
}
const deviceState = new DeviceState();

(function () {
	// Initial fetch and load every 10s
	deviceState.fetchTemperatureHistory15Min();
	setInterval(() => {
		deviceState.fetchTemperatureHistory15Min();
	}, 10000);

	if (!!window.EventSource) {
		var source = new EventSource(apiUrl("/events"));

		source.addEventListener(
			"temperature",
			e => {
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
			e => {
				var data = JSON.parse(e.data);
				runInAction(() => {
					deviceState.weight = data.value;
				});
			},
			false
		);

		source.addEventListener(
			"open",
			e => {
				// Connection was opened.
				console.log("Device stream connection opened");
			},
			false
		);

		source.addEventListener(
			"error",
			e => {
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
