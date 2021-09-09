const config = require('./config.json');
const fetch = require('node-fetch');
const { ExplorerApi } = require("atomicassets");
const api = new ExplorerApi(config.endpoints.atomicassets, "atomicassets", { fetch });
const fs = require('fs');
const resultsPerPage = 1000;

(async () => {
    let page = 1;
    let allNfts = [];

    while (true) {
        console.log(`Getting page ${page} with a limit of ${resultsPerPage}`);
        const data = await api.getAssets(config.mintsConfig.filter, page, resultsPerPage).then(x => x).catch(error => ({ success: false, error }));

        console.log(`Request done...`);

        if (data.success === false) {
            console.log(`[!!!] An error occured on page ${page}, possible error: ${data.error}`);
            break;
        }

        if (data.length == 0) {
            console.log(`No data received, reached the end...`);
            break;
        }

        allNfts = allNfts.concat(data.filter(x => x.template_mint !== "0").filter(x => parseInt(x.template_mint) >= config.mintsConfig.minMint && parseInt(x.template_mint) <= config.mintsConfig.maxMint).map(x => ({
            asset_id: parseInt(x.asset_id),
            template_id: parseInt(x.template?.template_id),
            mint: parseInt(x.template_mint),
        })));

        page++;

        // We are sorting by mints number... if we get anything above the max mint, abort
        if (data.filter(x => parseInt(x.template_mint) > config.mintsConfig.maxMint).length >= 1) {
            console.log(`Aborting got all mints up to ${config.mintsConfig.maxMint}`);
            break;
        }
    }

    console.log(allNfts);
    console.log(allNfts.length);

    fs.writeFileSync('./mint.cache.json', JSON.stringify(allNfts));
})();