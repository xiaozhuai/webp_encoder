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

window.getImageDataCanvas = document.createElement('canvas');
window.getImageDataCanvasCtx = getImageDataCanvas.getContext('2d', {
    willReadFrequently: true,
});

function getImageData(image) {
    window.getImageDataCanvas.width = image.naturalWidth;
    window.getImageDataCanvas.height = image.naturalHeight;
    window.getImageDataCanvasCtx.drawImage(image, 0, 0);
    return window.getImageDataCanvasCtx.getImageData(0, 0, image.naturalWidth, image.naturalHeight);
}

async function encodeWebp(frames, fileOptions = {}) {
    let m = await WebpEncoder();
    let encoder = new m.WebpEncoder();
    encoder.Init();
    encoder.SetOptions(fileOptions);

    for (let frame of frames) {
        let image = await loadImage(frame.src);
        let imageData = getImageData(image);
        let rgbaPixels = new Uint8Array(imageData.data.buffer);
        encoder.AddFrame(rgbaPixels, image.naturalWidth, image.naturalHeight, frame.options);
    }
    let bytes = encoder.Encode();
    let blob = new Blob([bytes], {type: 'image/webp'});
    encoder.Release();
    return blob;
}

const frameOptions = {
    duration: 100,
    lossless: false,
    quality: 100,
    method: 0,
};
const frames = [
    {
        src: 'frames/frame_0.jpg',
        options: frameOptions,
    },
    {
        src: 'frames/frame_1.jpg',
        options: frameOptions,
    },
    {
        src: 'frames/frame_2.jpg',
        options: frameOptions,
    },
    {
        src: 'frames/frame_3.jpg',
        options: frameOptions,
    },
];

async function main() {
    let imageContainer = document.getElementsByClassName('image-container')[0];
    for (let frame of frames) {
        let image = document.createElement('img');
        image.src = frame.src;
        image.className = 'image';
        imageContainer.appendChild(image);
    }
    let webp = document.getElementById('webp');
    let blob = await encodeWebp(frames, {
        min_size: true,
        loop: 0,
        kmax: 0,
        kmin: 0,
        mixed: true,
    });
    let url = URL.createObjectURL(blob);
    webp.src = url;
    webp.className = '';
    let loading = document.getElementById('loading');
    loading.className = 'hidden';
}

main().then();
