const { getApi } = require('./utils/api');
const config = require('./config.json');
const cache = require('./mint.cache.json');

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

(async () => {
    const api = getApi(config.endpoints.wax, config.auth.key);
    let i = 1;
    for (const chunk of chunks) {
        console.log(`Saving chunk ${i}`);
        await api.transact({
            actions: [{
                account: config.target.contract,
                name: 'addmint',

                authorization: [{
                    actor: config.auth.address,
                    permission: 'active',
                }],

                data: {
                    index: i++,
                    template_id: chunk.template_id,
                    new_mints: chunk.mints,
                },
            }]
        }, {
            blocksBehind: 3,
            expireSeconds: 30,
        });
    }
})();