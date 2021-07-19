export function sleep(ms: number): Promise<void>
{
    return new Promise((resolve) => setTimeout(resolve, ms));
}

export function timeout<T, TReason = undefined>(ms: number, reason?: TReason): Promise<T>
{
    return new Promise<T>((resolve, reject) => setTimeout(() => reject(reason), ms));
}

export function promiseWithTimeout<T, TReason>(promise: Promise<T>, ms: number, reason?: TReason): Promise<T>
{
    return Promise.race([promise, timeout<T, TReason>(ms, reason)]);
}