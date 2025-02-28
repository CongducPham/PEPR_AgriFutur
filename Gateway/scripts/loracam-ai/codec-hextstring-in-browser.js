const new_codec_script = `
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
`;

const jsonBody = {
  id: "application/hexstring",
  name: "HEXSTRING",
  mime: "application/javascript",
  script: new_codec_script
};

async function POST_custom_WaziGate_CODEC() {

  try {
    const POSTRequestResponse = await fetch("/codecs", {
      method: "POST",
      body: JSON.stringify(jsonBody),
      headers: {
        "Content-type" : "application/json"
      }

    });
    const POSTRequestResponseContent = await POSTRequestResponse.text();
    console.log("New codec id:", POSTRequestResponseContent);
  }
  catch (err) {
		console.error(`Error at POST_custom_WaziGate_CODEC : ${err}`);
		throw err;
	}
}

POST_custom_WaziGate_CODEC()