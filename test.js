const binding = require('bindings')('rpi_ws281x_smi');

console.log("running test")
binding.init(256)

let nr=0;

let count=0;
let start=0;

//performance testing
while(1) {

    binding.clear();
    nr = (nr + 1) % 256;
    for (let l = 0; l < 256; l++) {
        for (let c = 0; c < 16; c++) {
            if (l === nr)
                binding.setPixel(c, l, 0x40)
            else
                binding.setPixel(c, l, 0x0);
        }
    }
   binding.send();

    count++;
    if (Date.now()-start>1000)
    {
        console.log(count, " fps");
        count=0;
        start=Date.now();
    }

}

