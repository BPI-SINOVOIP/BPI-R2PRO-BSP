import { TestBed } from '@angular/core/testing';

import { Json2xmlService } from './json2xml.service';

describe('Json2xmlService', () => {
  let service: Json2xmlService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(Json2xmlService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
