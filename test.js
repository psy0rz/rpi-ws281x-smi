const binding = require('bindings')('rpi_ws281x_smi');

console.log("running test")
binding.init(256)

let nr=0;
setInterval(()=>{

    binding.clear();
    nr=(nr+1)%256;
    // for (let l=0; l<256; l++) {
    //     for (let c = 0; c < 8; c++) {
    //
    //     if (nr == l)
    // if (nr!=3)
         binding.setPixel(5, 0, nr<<16);
         //     else
         //         binding.setPixel(c, l, 0x0);
         // }
     // }
    binding.send();

},1000/60);

