const { generateSalt } = require('./utils/generateSalt');
const { getApi } = require('./utils/api');
const config = require('./config.json');

(async () => {
    const salt = await generateSalt();

    const api = getApi(config.endpoints.wax, config.auth.key);

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