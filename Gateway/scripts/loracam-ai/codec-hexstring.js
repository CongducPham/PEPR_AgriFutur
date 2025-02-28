function Decoder(bytes, port) {

  // Convert bytes to hexadecimal string
  var hexString = bytes2HexString(bytes);

  // Return the decoded values as a JSON object
  return {
    imagePkt: hexString
  };
}

// Helper function to convert byte array to hexadecimal string
function bytesToHex(bytes) {
  return bytes.map(function(byte) {
    return ('0' + (byte & 0xFF).toString(16)).slice(-2);
  }).join('');
}

function bytes2HexString(arrBytes) {
    var str = ''
    for (var i = 0; i < arrBytes.length; i++) {
        var tmp
        var num = arrBytes[i]
        if (num < 0) {
            tmp = (255 + num + 1).toString(16)
        } else {
            tmp = num.toString(16)
        }
        if (tmp.length === 1) {
            tmp = '0' + tmp
        }
        str += tmp
    }
    return str
}

//fetch("/codecs/602bc61f4b9f612bf0d6969b", {
//  method: "DELETE"
//});