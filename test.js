function parseInput(input) {
    let ret = null;
    let pattern = /(-?[\w._-]+)/g;
    for (let matches = pattern.exec(input); matches != null; matches = pattern.exec(input)) {
        if (null === ret) {
            if (matches[1].charAt(0) != '-') {
                ret = new Object();
                ret.command = matches[1];
                ret.options = new Array();
            }
            else {
                break;
            }
        }
        else {
            ret.options.push(matches[1]);
        }
    }
    return ret;
}

function isOption(str) {
    if (str.charAt(0) == '-') {
        return true;
    }
    else {
        return false;
    }
}

function readFileSize(fileName)
{
    let file = std.fopen(fileName, "rb");
    if(file != null)
    {
        std.fseek(file, 0, std.SEEK_END);
        let len = std.ftell(file);
        console.log(len);
        std.fseek(file, 0, std.SEEK_SET);
        std.fclose(file);
    }
    else
    {
        console.log("No such file or directory.");
    }
}

function readFile(fileName)
{
    let file = std.fopen(fileName, "rb");
    if(file != null)
    {
        std.fseek(file, 0, std.SEEK_END);
        let size = std.ftell(file);
        std.fseek(file, 0, std.SEEK_SET);
        let buff = new ArrayBuffer(size + 1);
        let len = std.fread(buff, 1, size, file);
        if(len > 0)
        {
            let array = new Uint8Array(buff);
            array[len] = 0;
            let str = String.fromCharCode.apply(null, array);
            console.log(str);
        }
        std.fclose(file);
    }
    else
    {
        console.log("No such file or directory.");
    }
}

function file(options) {
    let option = "";
    for (const str of options) {
        if (isOption(str)) {
            switch (str) {
                case "-s":
                    {
                        option = str;
                        console.log("-s");
                        break;
                    }
                case "-w":
                    {
                        option = str;
                        console.log("-w");
                        break;
                    }
                default:
                    {
                        console.log("Unknown option.");
                        break;
                    }
            }
        }
        else {
            switch (option) {
                case "-s":
                    {
                        readFileSize(str);
                        break;
                    }
                case "-w":
                    {
                        break;
                    }
                default:
                    {
                        readFile(str);
                        break;
                    }
            }
        }
    }
}

let isRunning = true;
for (; isRunning;) {
    console.log("Please enter...");
    let input = std.scanf();
    let result = parseInput(input);
    if (result != null) {
        switch (result.command) {
            case "q":
                {
                    isRunning = false
                    break;
                }
            case "file":
                {
                    file(result.options);
                    break;
                }
            default:
                {
                    console.log("Command not Found.");
                    break;
                }
        }
    }
}
