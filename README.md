# webp_encoder

A cross platform C++ (win, linux, unix, macos, wasm) library that encodes sequence image frames into WebP format.

## Online Page

See online page [https://xiaozhuai.github.io/webp_encoder/](https://xiaozhuai.github.io/webp_encoder/)

## Usage

### C++

```cpp
#include <string>
#include <vector>
#include <webp_encoder.hpp>

int main() {
    WebpEncoder encoder;
    encoder.Init(file_options);
    WebpFileOptions file_options{
            .minimize = true,
            .loop = 0,
            .kmax = 0,
            .kmin = 0,
            .mixed = true,
    };
    WebpFrameOptions frame_options{
            .duration = 100,
            .lossless = false,
            .quality = 100.0f,
            .method = 0,
    };

    // Push frames in RGBA format
    encoder.Push(frame_00001, width, height, frame_options);
    encoder.Push(frame_00002, width, height, frame_options);
    encoder.Push(frame_00003, width, height, frame_options);
    frame_options.duration = 200;
    encoder.Push(frame_00004, width, height, frame_options);
    // Add more frames here...

    encoder.Write("test.webp");
    encoder.Release();
}
```

### Javascript

```js
async function loadImage(url) {
    const image = new Image();
    await new Promise((resolve, reject) => {
        image.onload = resolve;
        image.onerror = () => {
            reject(new Error(`Error load image ${url}`));
        };
        image.src = url;
    });
    return image;
}

let getImageDataCanvas = null;
let getImageDataCanvasCtx = null;
async function getImageData(image) {
    if (!getImageDataCanvas) {
        getImageDataCanvas = document.createElement('canvas');
        getImageDataCanvasCtx = getImageDataCanvas.getContext('2d', {
            willReadFrequently: true,
        });
    }
    let canvas = getImageDataCanvas;
    let context = getImageDataCanvasCtx;
    canvas.width = image.naturalWidth;
    canvas.height = image.naturalHeight;
    context.drawImage(image, 0, 0);
    return context.getImageData(0, 0, image.naturalWidth, image.naturalHeight);
}

const WEBP_ENCODE_RET_UINT8ARRAY = 0;
const WEBP_ENCODE_RET_BLOB = 1;
const WEBP_ENCODE_RET_URL = 2;
/**
 *
 * @param frames            Array<{src: string, options: Object}>
 * @param fileOptions       Object
 * @param returnType        0: Uint8Array
 *                          1: {blob, size}
 *                          2: {url, size}
 * @returns {Promise<Uint8Array|{blob: Blob, size: number}|{url: string, size: number}>}
 */
async function encodeWebp(frames, fileOptions = {}, returnType = 0) {
    let m = await WebpEncoder();
    let encoder = new m.WebpEncoder();
    encoder.init(fileOptions);

    for (let frame of frames) {
        let image = await this.loadImage(frame.src);
        let imageData = this.getImageData(image);
        let rgbaPixels = new Uint8Array(imageData.data.buffer);
        encoder.push(rgbaPixels, image.naturalWidth, image.naturalHeight, frame.options);
    }
    let bytes = encoder.encode();
    switch (returnType) {
        case 0: {
            bytes = new Uint8Array(bytes);
            encoder.release();
            return bytes;
        }
        case 1: {
            let blob = new Blob([bytes], {type: 'image/webp'});
            let size = bytes.length;
            encoder.release();
            return {blob, size};
        }
        case 2: {
            let blob = new Blob([bytes], {type: 'image/webp'});
            let size = bytes.length;
            encoder.release();
            let url = URL.createObjectURL(blob);
            return {url, size};
        }
        default: {
            throw new Error(`Invalid return type ${returnType}`)
        }
    }
}

async function main() {
    let fileOptions = {
        loop: 0,
        kmax: 0,
        kmin: 0,
        minimize: true,
        mixed: true,
    };
    let frameOptions = {
        duration: 100,
        quality: 100,
        method: 0,
        lossless: false,
    };
    let images = [
        {
            src: 'https://xiaozhuai.github.io/webp_encoder/frames/frame_0.jpg',
            options: frameOptions,
        },
        {
            src: 'https://xiaozhuai.github.io/webp_encoder/frames/frame_1.jpg',
            options: frameOptions,
        },
        {
            src: 'https://xiaozhuai.github.io/webp_encoder/frames/frame_2.jpg',
            options: frameOptions,
        },
        {
            src: 'https://xiaozhuai.github.io/webp_encoder/frames/frame_3.jpg',
            options: {...frameOptions, duration: 200},
        },
    ];

    /**
     * Got webp blob url and length in byte
     */
    let {url, size} = encodeWebp(frames, fileOptions, WEBP_ENCODE_RET_URL);
}

main();
```
