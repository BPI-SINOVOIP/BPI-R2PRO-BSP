import { TestBed } from '@angular/core/testing';

import { PublicFuncService } from './public-func.service';

describe('PublicFuncService', () => {
  let service: PublicFuncService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(PublicFuncService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
