import * as std from "std";

export function debugLog(name, ...args) {
  const debug = std.getenv('QJS_DEBUG');
  if (debug) {
    if (debug.split(/,/).includes(name)) {
      console.log(`DEBUG: ${name}: ${inspect(...args)}`);
    }
  }
}
export function log(...args) {
  std.printf(`${args[0]}\n`, args.slice(1));
}
export function inspect(item) {
  return JSON.stringify(item);
}