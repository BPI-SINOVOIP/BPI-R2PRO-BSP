import { TestBed } from '@angular/core/testing';

import { LockServiceService } from './lock-service.service';

describe('LockServiceService', () => {
  let service: LockServiceService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(LockServiceService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
