importScripts('./webp_encoder.js');

let ModulePromise = WebpEncoder();
let encoder = null;

self.onmessage = async (e) => {
    const { id, type, payload } = e.data;
    try {
        const module = await ModulePromise;
        switch (type) {
            case 'init':
                encoder = new module.WebpEncoder();
                encoder.init(payload.options);
                postMessage({ id });
                break;

            case 'push': {
                const pixels = new Uint8Array(payload.pixels);
                encoder.push(pixels, payload.width, payload.height, payload.options);
                postMessage({ id });
                break;
            }

            case 'encode': {
                const result = encoder.encode();
                postMessage({ id, result });
                break;
            }
        }
    } catch (err) {
        postMessage({ id, error: err.message });
    }
};
