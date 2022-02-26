import apiClient from "@/api/apiClient";

// initial state
// shape: [{ id, quantity }]
const state = () => ({
	temperature: "N/A",
	temperatureHistory: [],
	weight: "N/A",
	weightHistory: []
})

// getters
const getters = {
	cartProducts: (state, getters, rootState) => {
		return state.items.map(({ id, quantity }) => {
			const product = rootState.products.all.find(product => product.id === id)
			return {
				title: product.title,
				price: product.price,
				quantity
			}
		})
	},

	cartTotalPrice: (state, getters) => {
		return getters.cartProducts.reduce((total, product) => {
			return total + product.price * product.quantity
		}, 0)
	}
}














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
	// Initial fetch and load every second
	deviceState.fetchTemperature();
	setInterval(() => {
		deviceState.fetchTemperature();
	}, 1000);
	// Initial fetch and load every 10s
	deviceState.fetchTemperatureHistory(15);
	setInterval(() => {
		deviceState.fetchTemperatureHistory(15);
	}, 10000);

	// Initial fetch and load every second
	deviceState.fetchWeight();
	setInterval(() => {
		deviceState.fetchWeight();
	}, 1000);
	// Initial fetch and load every 10s
	deviceState.fetchWeightHistory(15);
	setInterval(() => {
		deviceState.fetchWeightHistory(15);
	}, 10000);

	if (window.EventSource) {
		const source = new EventSource(apiUrl("/events"));

		source.addEventListener(
			"temperature",
			(e) => {
				const data = JSON.parse(e.data);
				runInAction(() => {
					deviceState.temperature = data.value;
				});
			},
			false
		);

		source.addEventListener(
			"weight",
			(e) => {
				const data = JSON.parse(e.data);
				runInAction(() => {
					deviceState.weight = data.value;
				});
			},
			false
		);

		source.addEventListener(
			"open",
			() => {
				// Connection was opened.
				console.log("Device stream connection opened");
			},
			false
		);

		source.addEventListener(
			"error",
			(e) => {
				if (e.readyState === EventSource.CLOSED) {
					console.log("Device stream connection closed");
				}
			},
			false
		);

		source.close();
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
