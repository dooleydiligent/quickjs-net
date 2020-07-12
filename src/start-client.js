import { inspect, log } from './util/debug.js';
import { Client } from './socket/client.js';

const client = new Client(7981, 'localhost', true);
log(`client is %s`, inspect(client));
client.send(`Hello server`);
client.close('because');
