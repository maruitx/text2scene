scene=<text>
output="StanfordSceneDatabase\n";
output += "modelCount " + scene.length + "\n";
for(const o of scene) {
output += "newModel " + o.index + " " + o.modelId.replace("wss.", "") + "\n";
output += "transform " + o.transform.data + "\n";
output += "position 0 0 0" + "\n";
output += "frontDir 1 0 0" + "\n";
output += "upDir 0 0 1" + "\n";
}