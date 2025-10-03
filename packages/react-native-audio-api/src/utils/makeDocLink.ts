import { baseUrl } from './constants';

export default function makeDocLink(path: string) {
  return `${baseUrl}${path}`;
}
