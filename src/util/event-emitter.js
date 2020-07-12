import { debugLog, inspect, log } from './debug.js';
const id = 'event-emitter';

export class EventEmitter {
  constructor() {
    debugLog(id, 'constructor()');
    this.events = {};
  }
  emit(name, ...args) {
    debugLog(id, `emit(${name}, ${args})`);
    if (!this.events[name]) {
      return;
    }
    for (let callback of this.events[name]) {
      callback(...args);
    }
  }
  off(name, callback) {
    debugLog(id, `off(${name}, ${callback})`);
    if (!this.events[name]) {
      return;
    }
    this.events[name] = this.events[name]
      .filter(cb => cb !== callback);
  }
  on(name, callback) {
    debugLog(id, `on(${name}, ${callback})`);
    if (!this.events[name]) {
      this.events[name] = [];
    }
    this.events[name].push(callback);
  }
  once(name, callback) {
    debugLog(id, `once(${name}, ${callback})`);
    const onceCallback = (...args) => {
      callback(...args);
    };
    this.on(name, onceCallback);
    this.off(name, onceCallback);
  }
}