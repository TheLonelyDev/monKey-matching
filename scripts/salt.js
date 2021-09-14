const { generateSalt } = require('./utils/generateSalt');
const { getApi } = require('./utils/api');
const fetch = require('node-fetch');
const config = require('./config.json');

let endpoints = [];

(async () => {
    endpoints = config.endpoints.wax ?? (await fetch('http://waxmonitor.cmstats.net/api/endpoints?format=json&type=api').then(x => x.json())).filter(({ weight }) => weight > 5).map(({ node_url }) => node_url);
    console.log('Got endpoints', endpoints);

    const salt = await generateSalt();

    const api = getApi(endpoints[Math.floor(Math.random() * endpoints.length)], config.auth.key);

    await api.transact({
        actions: [{
            account: config.target.contract,
            name: 'setsalt',

            authorization: [{
                actor: config.auth.address,
                permission: 'active',
            }],

            data: {
                salt,
            },
        }]
    }, {
        blocksBehind: 3,
        expireSeconds: 30,
    });

    console.log(`Set salt to ${salt}`);
})();