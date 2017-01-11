function escapeRegExp(str) {
    return str.replace(/([.*+?^=!:${}()|\[\]\/\\])/g, "\\$1");
}
function replaceAll(str, find, replace) {
  return str.replace(new RegExp(escapeRegExp(find), 'g'), replace);
}
output="StanfordSceneDatabase\n";
output += "modelCount " + scene.length + "\n";
for(const o of scene) {
output += "newModel " + o.index + " " + o.modelId.replace("wss.", "") + "\n";
output += "transform " + replaceAll(o.transform.data.toString(), ",", " ") + "\n";
output += "position 0 0 0" + "\n";
output += "frontDir 1 0 0" + "\n";
output += "upDir 0 0 1" + "\n";
}