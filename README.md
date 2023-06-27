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
            .exact = false,
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
const WebpEncoder = {
    ENCODE_RET_UINT8ARRAY: 0,
    ENCODE_RET_BLOB: 1,
    ENCODE_RET_URL: 2,
    _module: null,
    _getImageDataCanvas: null,
    _getImageDataCanvasCtx: null,
    async loadImage(url) {
        const image = new Image();
        await new Promise((resolve, reject) => {
            image.onload = resolve;
            image.onerror = () => {
                reject(new Error(`Error load image ${url}`));
            };
            image.src = url;
        });
        return image;
    },
    getImageData(image) {
        if (!this._getImageDataCanvas) {
            this._getImageDataCanvas = document.createElement('canvas');
            this._getImageDataCanvasCtx = this._getImageDataCanvas.getContext('2d', {
                willReadFrequently: true,
            });
        }
        let canvas = this._getImageDataCanvas;
        let context = this._getImageDataCanvasCtx;
        canvas.width = image.naturalWidth;
        canvas.height = image.naturalHeight;
        context.drawImage(image, 0, 0);
        return context.getImageData(0, 0, image.naturalWidth, image.naturalHeight);
    },
    /**
     *
     * @param frames            Array<{src: string, options: Object}>
     * @param fileOptions       Object
     * @param callback          function(progress: number)
     * @param returnType        0: Uint8Array
     *                          1: {blob, size}
     *                          2: {url, size}
     * @returns {Promise<Uint8Array|{blob: Blob, size: number}|{url: string, size: number}>}
     */
    async encode(frames, callback, fileOptions = {}, returnType = 0) {
        await callback(0);
        if (!this._module) {
            this._module = await WebpEncoderWasm();
        }
        let encoder = new this._module.WebpEncoder();
        encoder.init(fileOptions);
        await callback(5);

        let c = 0;
        for (let frame of frames) {
            let image = await this.loadImage(frame.src);
            let imageData = this.getImageData(image);
            let rgbaPixels = new Uint8Array(imageData.data.buffer);
            encoder.push(rgbaPixels, image.naturalWidth, image.naturalHeight, frame.options);
            c++;
            await callback(c / frames.length * 90 + 5);
        }

        await callback(95);
        let bytes = encoder.encode();
        switch (returnType) {
            case 0: {
                bytes = new Uint8Array(bytes);
                encoder.release();
                await callback(100);
                return bytes;
            }
            case 1: {
                let blob = new Blob([bytes], {type: 'image/webp'});
                let size = bytes.length;
                encoder.release();
                await callback(100);
                return {blob, size};
            }
            case 2: {
                let blob = new Blob([bytes], {type: 'image/webp'});
                let size = bytes.length;
                encoder.release();
                let url = URL.createObjectURL(blob);
                await callback(100);
                return {url, size};
            }
            default: {
                throw new Error(`Invalid return type ${returnType}`)
            }
        }
    },
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
        exact: false,
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
    let {url, size} = WebpEncoder.encode(frames, progress => {
        console.log(`progress: ${progress}%`);
    }, fileOptions, webpEncoder.ENCODE_RET_URL);
}

main();
```
