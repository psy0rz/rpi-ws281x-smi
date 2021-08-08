const binding = require('bindings')('rpi_ws281x_smi');

console.log("running test")
binding.ledsInit(256)

let nr=0;
setInterval(()=>{

    nr=(nr+1)%60;
    for (let l=0; l<256; l++) {
        for (let c = 0; c < 8; c++) {

            // if (nr == l)
            //     binding.ledsSetPixel(c, l, 0x1);
            // else
                binding.ledsSetPixel(c, l, 0x0);
        }
    }
    binding.ledsSend();

},1000/60);

