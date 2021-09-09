const { PrivateKey, sign } = require('eosjs-ecc');

const generateSalt = async () => {
    const key = await PrivateKey.randomKey();
    const nonce = Math.floor(Math.random() * (2 ** 32));

    const signature = sign(
        `match-a-monkey-${nonce}`,
        key.toWif(),
        'utf8',
    ).toString();

    return signature;
}

module.exports = {
    generateSalt,
};