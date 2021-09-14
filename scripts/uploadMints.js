const { getApi } = require('./utils/api');
const config = require('./config.json');
const cache = require('./mint.cache.json');
const fetch = require('node-fetch');

const groups = {};

// groups
const chunks = [];
const chunkSize = 100;

cache.forEach(x => {
    if (groups[x.template_id] === undefined) {
        groups[x.template_id] = [];
    }

    groups[x.template_id].push(x);
});

for (const key in groups) {
    const value = groups[key];

    const size = value.length;
    for (let i = 0; i < size; i += chunkSize) {
        chunks.push({
            template_id: key,
            mints: value.slice(i, i + chunkSize).map(x => ({
                asset_id: x.asset_id,
                mint: x.mint,
            })),
        });
    }
};
console.log(`Chunked data into ${chunks.length} chunks`);

let endpoints = [];

const action = async ({ template_id, mints }, index) => {
    const endpoint = endpoints[Math.floor(Math.random() * endpoints.length)];
    console.log('Using endpoint', endpoint);
    const api = getApi(endpoint, config.auth.key);

    return api.transact({
        actions: [{
            account: config.target.contract,
            name: 'addmint',
    
            authorization: [{
                actor: config.auth.address,
                permission: 'active',
            }],
    
            data: {
                index,
                template_id: template_id,
                new_mints: mints,
            },
        }]
    }, {
        blocksBehind: 3,
        expireSeconds: 30,
    });
};

(async () => {
    endpoints = config.endpoints.wax ?? (await fetch('http://waxmonitor.cmstats.net/api/endpoints?format=json&type=api').then(x => x.json())).filter(({ weight }) => weight > 5).map(({ node_url }) => node_url);
    console.log('Got endpoints', endpoints);

    const softStart = 0;

    for (let index = softStart; index < chunks.length; index) {
        console.log('Saving chunk', index);

        await action(chunks[index], index)
            .then(() => index++)
            .catch(x => console.log('Got error, retrying', x));
    }
})();