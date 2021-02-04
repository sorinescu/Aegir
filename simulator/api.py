import connexion
import flask
import json
import queue
import random
import threading
import time

app = connexion.FlaskApp(__name__, specification_dir='openapi/',
                         arguments={'global': 'global_value'})

# Simulate the device-local time by using random values
local_t0 = time.time()

temp_history = []


class MessageAnnouncer:
    '''
    See https://maxhalford.github.io/blog/flask-sse-no-deps/
    '''

    def __init__(self):
        self.listeners = []

    def listen(self):
        q = queue.Queue(maxsize=5)
        self.listeners.append(q)
        return q

    def announce(self, msg):
        for i in reversed(range(len(self.listeners))):
            try:
                self.listeners[i].put_nowait(msg)
            except queue.Full:
                del self.listeners[i]

announcer = MessageAnnouncer()


def device_time():
    return int((time.time() - local_t0) * 1000)


def go_home():
    return flask.send_from_directory('../data', 'index.html')


def device_time_get():
    return device_time()


def temperature_get():
    return temp_history[-1] or 50.0


def temperature_history_get():
    return {
        'ts': device_time(),
        'values': temp_history
    }


@app.route('/events', methods=['GET'])
def listen():
    # SSE stream
    def stream():
        messages = announcer.listen()  # returns a queue.Queue
        while True:
            msg = messages.get()  # blocks until a new message arrives
            yield msg

    return flask.Response(stream(), mimetype='text/event-stream')


def send_temperature_event():
    '''
    Send a random temperature reading every second
    '''
    threading.Timer(1.0, send_temperature_event).start()

    data = {
        'ts': device_time(),
        'value': random.randint(0, 100)
    }
    temp_history.append(data)

    msg = f"event: temperature\ndata: {json.dumps(data)}\n\n"
    announcer.announce(msg)


send_temperature_event()

app.add_api('api.yaml', arguments={'api_local': 'local_value'})
app.run(port=8080)
