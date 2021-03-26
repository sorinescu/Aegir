const applicationServerPublicKey = 'BN8PeALxQEBCSBf5hBmBCD-ta51WtMb-ZYIGiYjIYVWWehT5jP7g_Ln74KjanL_xHyImaQA-gXdL3Brs_uKM42E';

var isPushEnabled = false;

function urlBase64ToUint8Array(base64String) {
    var padding = '='.repeat((4 - base64String.length % 4) % 4);
    var base64 = (base64String + padding)
        .replace(/\-/g, '+')
        .replace(/_/g, '/');

    var rawData = window.atob(base64);
    var outputArray = new Uint8Array(rawData.length);

    for (var i = 0; i < rawData.length; ++i) {
        outputArray[i] = rawData.charCodeAt(i);
    }
    return outputArray;
}

function sendSubscriptionToServer(subscription) {
    fetch("/push/subscription", {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(subscription)
        })
        .then(response => console.log(response))
        .catch(e => console.error(e));
}

function subscribe() {
    navigator.serviceWorker.ready.then(serviceWorkerRegistration => {
        serviceWorkerRegistration.pushManager.subscribe({
                userVisibleOnly: true,
                applicationServerKey: urlBase64ToUint8Array(applicationServerPublicKey)
            })
            .then(subscription => {
                console.log('Got push subscription: ', subscription);

                // The subscription was successful
                isPushEnabled = true;

                // TODO: Send the subscription.endpoint to your server
                // and save it to send a push message at a later date
                return sendSubscriptionToServer(subscription);
            })
            .catch(e => {
                if (Notification.permission === 'denied') {
                    // The user denied the notification permission which
                    // means we failed to subscribe and the user will need
                    // to manually change the notification permission to
                    // subscribe to push messages
                    console.warn('Permission for Notifications was denied');
                } else {
                    // A problem occurred with the subscription; common reasons
                    // include network errors, and lacking gcm_sender_id and/or
                    // gcm_user_visible_only in the manifest.
                    console.error('Unable to subscribe to push.', e);
                }
            });
    });
}

// Once the service worker is registered set the initial state
function initialisePushState() {
    subscribe();

    // Are Notifications supported in the service worker?
    if (!('showNotification' in ServiceWorkerRegistration.prototype)) {
        console.warn('Notifications aren\'t supported.');
        return;
    }

    // Check the current Notification permission.
    // If its denied, it's a permanent block until the
    // user changes the permission
    if (Notification.permission === 'denied') {
        console.warn('The user has blocked notifications.');
        return;
    }

    // Check if push messaging is supported
    if (!('PushManager' in window)) {
        console.warn('Push messaging isn\'t supported.');
        return;
    }

    // We need the service worker registration to check for a subscription
    navigator.serviceWorker.ready.then(serviceWorkerRegistration => {
        // Do we already have a push message subscription?
        serviceWorkerRegistration.pushManager.getSubscription()
            .then(subscription => {
                // Enable any UI which subscribes / unsubscribes from
                // push messages.
                if (!subscription) {
                    // We aren't subscribed to push, so set UI
                    // to allow the user to enable push
                    console.log('We aren\'t subscribed to push');
                    return;
                }

                // Keep your server in sync with the latest subscriptionId
                sendSubscriptionToServer(subscription);

                // Set your UI to show they have subscribed for
                // push messages
                isPushEnabled = true;
            })
            .catch(function(err) {
                console.warn('Error during getSubscription()', err);
            });
    });

    console.log('Service worker state initialised.');
}

// Check that service workers are supported, if so, progressively
// enhance and add push messaging support, otherwise continue without it.
if ('serviceWorker' in navigator) {
    // The script must be in '/', otherwise 'navigator.serviceWorker' is never ready 
    // (because the script can only get notifications from '/js/*' scope).
    navigator.serviceWorker.register('/service-worker.js', {
            scope: '/'
        })
        .then(initialisePushState)
        .catch(error => {
            // registration failed
            console.log('Service worker registration failed with ' + error);
        });
} else {
    console.warn('Service workers aren\'t supported in this browser.');
}